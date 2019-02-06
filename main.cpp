#include <fstream>
#include <stdexcept>

#include <AnyCom/Core/inc/anycomplatform.hpp>

#include <boost/assert.hpp>
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

/**
 * @brief Local assert helper macro for wrong row parameters
 * @param aExpression Assertion expression which must be evaluate to true.
 * @param aMessage Error message
 */
#ifdef ANYCOM_DEBUG
    #define ANYCOM_EIGEN_SERIALIZE_ASSERT(aExpression, aMessage) \
        if(true) { \
            BOOST_ASSERT_MSG((aExpression), aMessage); \
        } else \
            (void)0
#else // release
    #define ANYCOM_EIGEN_SERIALIZE_ASSERT(aExpression, aMessage) \
        if(!(aExpression)) { \
            throw std::runtime_error(aMessage); \
        } else \
            (void)0
#endif


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
            (void) aVersion;

            Eigen::Index lRows = arMatrix.rows();
            Eigen::Index lCols = arMatrix.cols();

            // save / load meta data
            arArchive & BOOST_SERIALIZATION_NVP(lRows);
            arArchive & BOOST_SERIALIZATION_NVP(lCols);

            // resize matrix if dynamic-sized
            constexpr auto lcResizeRowsPossible = arMatrix.RowsAtCompileTime == Eigen::Dynamic;
            auto lRowsEqual = _Rows == lRows ;
            ANYCOM_EIGEN_SERIALIZE_ASSERT(lcResizeRowsPossible || (_Rows == lRows), "Unexpected number of rows found for fixed-sized type!");

            constexpr auto lcResizeColsPossible = arMatrix.ColsAtCompileTime == Eigen::Dynamic;
            auto lColsEqual = _Cols == lCols;
            ANYCOM_EIGEN_SERIALIZE_ASSERT(lcResizeColsPossible || (_Cols == lCols), "Unexpected number of columns found for fixed-sized type!");

            // if we got here, the matrix is either fixed-size (and the sizes are correct), which is a NOP,
            // or, we have a dynamically-sized matrix which should be resized (or not)
            arMatrix.resize(lcResizeRowsPossible ? lRows : Eigen::NoChange,
                            lcResizeColsPossible ? lCols : Eigen::NoChange);

            // deserialize data if the matrix has any elements
            auto lMatrixSize = arMatrix.size();
            if(lMatrixSize > 0)
            {
                arArchive & boost::serialization::make_nvp("data", boost::serialization::make_array(arMatrix.data(), lMatrixSize));
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
