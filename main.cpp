#include <fstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/split_free.hpp>

#include <Eigen/Core>

// !! Conflicting include! Whenever the serialization wrapper for shared_ptrs is included
// the compilation fails!
// /usr/local/include/Eigen/src/Core/util/ForwardDeclarations.h:32:
// error: incomplete type ‘Eigen::internal::traits<boost::serialization::U>’ used in nested name specifier
// enum { has_direct_access = (traits<Derived>::Flags & DirectAccessBit) ? 1 : 0,
#include <boost/serialization/shared_ptr.hpp>

// Serialization methods for fixed-size Eigen::Matrix type
namespace boost {
    namespace serialization {
        template<
                class Archive,
                typename _Scalar,
                int _Rows,
                int _Cols,
                int _Options,
                int _MaxRows,
                int _MaxCols
                >
        inline void serialize(Archive & arArchive,
                              Eigen::Matrix<_Scalar,
                              _Rows,
                              _Cols,
                              _Options,
                              _MaxRows,
                              _MaxCols> & arMatrix,
                              const unsigned int aVersion)
        {
            boost::serialization::split_free(arArchive, arMatrix, aVersion);
        }

        template<
                class Archive,
                typename _Scalar,
                int _Rows,
                int _Cols,
                int _Options,
                int _MaxRows,
                int _MaxCols
                >
        inline void save(Archive & arArchive,
                         const Eigen::Matrix<_Scalar,
                         _Rows,
                         _Cols,
                         _Options,
                         _MaxRows,
                         _MaxCols> & arMatrix,
                         const unsigned int)
        {
            typedef typename Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols>::Index TEigenIndex;

            const TEigenIndex lRows = arMatrix.rows();
            const TEigenIndex lCols = arMatrix.cols();

            arArchive << lRows;
            arArchive << lCols;

            if(lRows > 0 && lCols > 0)
            {
                arArchive & boost::serialization::make_array(arMatrix.data(), arMatrix.size());
            }
        }

        template<
                class Archive,
                typename _Scalar,
                int _Rows,
                int _Cols,
                int _Options,
                int _MaxRows,
                int _MaxCols
                >
        inline void load(Archive & arArchive,
                         Eigen::Matrix<_Scalar,
                         _Rows,
                         _Cols,
                         _Options,
                         _MaxRows,
                         _MaxCols> & arMatrix,
                         const unsigned int)
        {
            typedef typename Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols>::Index TEigenIndex;

            TEigenIndex lRows, lCols;

            // deserialize meta data
            arArchive & lRows;
            arArchive & lCols;

            // do some error handling here

            if(lRows > 0 && lCols > 0)
            {
                // deserialize data
                arArchive & boost::serialization::make_array(arMatrix.data(), arMatrix.size());
            }
        }

    }
}

class TestClass
{
    public:
        TestClass()
        {
            // fill eigen
            m(0,0) = 3;
            m(1,0) = 2.5;
            m(0,1) = -1;
            m(1,1) = m(1,0) + m(0,1);
        }

    private:
        friend class boost::serialization::access;
        Eigen::Matrix2d m;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int)
        {
            ar & m;
        }
};


int main(void)
{
    using namespace boost::archive;

    // Serialize
    TestClass TestA;
    std::ofstream oss("test.log");
    {
        text_oarchive oa(oss);
        oa << TestA;
    }

    // deserialize now
    TestClass TestB;
    std::ifstream iss("test.log");
    {
        text_iarchive ia(iss);
        ia >> TestB;
    }
}
