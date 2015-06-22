#include <string>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>

#include <cstdlib>
#include <unistd.h>
#include <assert.h>


static uint64_t parse_size( std::string & str, uint64_t defsize )
{
        uint64_t        mult = 1;
        size_t          len = str.size();
        uint64_t        size = defsize;
        if ( str.length() == 0 )
        {
                return size;
        }
        switch ( str[len-1] ) {
        case 'k':
        case 'K':
                mult = 1024;
                len--;
                break;
        case 'm':
        case 'M':
                mult = 1024*1024;
                len--;
                break;
        case 'g':
        case 'G':
                mult = 1024*1024*1024;
                len--;
                break;
        default:
                break;
        }

        size = strtoul( str.substr( 0, len ).c_str(), nullptr, 0 ) * mult;
        return size < defsize ? defsize : size;
}

const char* optstr = "a:h";

static void usage(const char * name)
{
        printf("Usage : %s [options]\n", name);
        printf("        %s\n", name);
        printf("        %s -a type=merge,size=1m\n", name);
        printf("Valid options are: \n");
        printf("    -a algorithm_opt [...]\n");
        printf("                type    Type of alogrithm                       (default:insertion)\n"  );
        printf("                        insertion       Insertion sort\n"                               );
        printf("                        merge           Merge sort\n"                                   );
        printf("                size    input size                              (default:1K)\n"         );
        printf("    -h help menu\n");
        printf("\n");

}

enum algo_type {
        insertion,
        merge,
        invalid
};

// -a type=insertion,size=1k
static const std::string default_algo_type = "insertion";
struct algorithm_type_options
{
        std::string     type;
        uint64_t        size;

        algorithm_type_options()
                : type( default_algo_type )
                , size( 1024 )
        {
        }

        void parse_one( std::string & opt )
        {
                std::istringstream ss( opt );
                std::string token;

                if ( !std::getline( ss, token, '=' ) ) {
                        printf("get token failed\n");
                        return;
                }
                std::string value;
                if ( !std::getline( ss, value, '=' ) ) {
                        return;
                }

                if ( !token.compare("type") ) {
                        type = value;
                } else if ( !token.compare( "size" ) ) {
                        size = parse_size( value, 1 );
                }
        }

        void parse( std::string & opt_str)
        {
                std::istringstream ss( opt_str );
                std::string token;

                while(std::getline(ss, token, ',')) {
                        parse_one( token );
                }
        }

        algo_type algo()
        {
                if ( !type.compare( "insertion" ) ) {
                        return insertion;
                } else if ( !type.compare( "merge" ) ) {
                        return merge;
                } else {
                        return invalid;
                }


        }
};

template<typename impl_t>
impl_t rand()
{
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<impl_t> dis;
        return dis( gen );
}


template<typename impl_t>
void fill_rand( impl_t * p, uint64_t num )
{
        for ( uint64_t i = 0; i < num; *(p++) = rand<impl_t>(), i++ );
}

template<typename impl_t>
struct buffer_list
{
        typedef std::vector<impl_t *> buffer_pool;
        uint64_t        sz;
        uint64_t        num;
        buffer_pool     pool;


        buffer_list( uint64_t sz_, uint64_t num_ = 1)
                : sz( sz_ )
                , num( num_ )
                , pool( num, nullptr )
        {
                for ( auto & p : pool )
                {
                        p = new impl_t[sz];
                }
        }

        virtual ~buffer_list()
        {
                for ( auto & p : pool )
                {
                        delete [] p;
                }
        }

        impl_t * get( uint64_t idx = 0 )
        {
                return pool[idx];
        }
};

template<typename impl_t>
struct sort_algo
{
        virtual bool verify(impl_t * p, uint64_t len)
        {
                bool verified = true;
                for ( uint64_t i = 1; verified && i < len; i++ )
                {
                        verified = verified &&  ( p[i - 1] <= p[i] );
                }
                return verified;
        }
        virtual void sort( buffer_list<impl_t> * bl, uint64_t len ) = 0;
};

template< typename impl_t>
struct insertion_sort : sort_algo<impl_t>
{
        void shift(impl_t * p, uint64_t start, uint64_t end)
        {
                while(end != start)
                {
                        p[end] = p[end-1];
                        end--;
                }
        }

        virtual void sort(buffer_list<impl_t> * bl, uint64_t len)
        {
                impl_t * p = bl->get();
                for (uint64_t i = 1; i < len; i++) 
                {
                        impl_t c = p[i];
                        for (uint64_t j = 0; j < i; j++)
                        {
                                if (c < p[j]) {
                                        shift( p, j , i);
                                        p[j] = c;
                                        break;
                                }
                        }
                }

        }
};

template<typename impl_t>
struct merge_sort : sort_algo<impl_t>
{
        buffer_list<impl_t> *   bl;
        int                     max_depth;

        void shift(impl_t * p, uint64_t start, uint64_t end)
        {
                while(end != start)
                {
                        p[end] = p[end-1];
                        end--;
                }
        }

        int merge_area(int depth)
        {
                return max_depth & 0x1 ?
                        ( max_depth ^ depth ) & 0x1 : depth & 0x1;
        }

        impl_t * merge_buffer(int depth)
        {
                return bl->get( merge_area( depth ) );
        }

        impl_t * data_buffer(int depth)
        {
                return bl->get( (merge_area( depth ) ^ 0x1 ) & 0x1 );
        }

        void merge(uint64_t lo, uint64_t mid, uint64_t len, int depth)
        {
                impl_t * src = data_buffer( depth );
                impl_t * dst = merge_buffer( depth );
                uint64_t l = lo, r = mid;
                for (uint64_t i = lo; i < len+lo; i++)
                {
                        if (l < mid && r < lo+len) {
                                if (src[l] < src[r]) {
                                        dst[i] = src[l++];
                                } else{
                                        dst[i] = src[r++];
                                }
                        } else if (l < mid) {
                                dst[i] = src[l++];
                        } else {
                                dst[i] = src[r++];
                        }
                }

        }

        virtual bool verify(impl_t * p, uint64_t len)
        {
               return sort_algo<impl_t>::verify( merge_buffer(0), len );
        }

        virtual void sort_impl(uint64_t off, uint64_t len, int depth)
        {
                if ( len < 2 ) {
                        max_depth = depth;
                        return;
                }
                uint64_t lo = off, half = len / 2, mid = half + lo;
                sort_impl( lo, half, depth + 1);
                sort_impl( mid, len - half, depth + 1);
                merge( lo, mid, len, depth );
        }

        virtual void sort(buffer_list<impl_t> * bl_, uint64_t len)
        {
                bl = bl_;
                sort_impl(0, len, 0);
        }
};

typedef int sort_t;
typedef buffer_list<sort_t> sort_buffer;

int main(int argc, char **argv)
{
        algorithm_type_options type;
        std::string opt_str;
        sort_algo<sort_t>       * fn;
        buffer_list<sort_t>     * bl;

        char c;

        while ( ( c = getopt( argc, argv, optstr ) ) != -1 ) {
                switch(c) {
                case 'a':
                        opt_str = optarg;
                        type.parse( opt_str );
                        break;
                case 'h':
                default:
                        usage( argv[0] );
                        return 0;
                }
        }

        switch ( type.algo() )
        {
        case insertion:
                fn = new insertion_sort<sort_t>();
                bl = new buffer_list<sort_t>( type.size );
                break;
        case merge:
                fn = new merge_sort<sort_t>();
                bl = new buffer_list<sort_t>( type.size, 2 );
                break;
        default:
                usage( argv[0] );
                return 0;
        }

        std::cout << "using sort algorithm: " << type.type << " size " <<  type.size << std::endl;
        fill_rand<sort_t>( bl->get(), type.size );

        fn->sort( bl, type.size );
        std::cout << "verify : " << fn->verify( bl->get(), type.size ) << std::endl;

        delete bl;
        delete fn;

        return 0;

}
