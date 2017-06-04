#include <gtest/gtest.h>

#include "test/mixxxtest.h"

#include "library/trackcollection.h"

#include "library/queryutil.h"
#include "util/db/sqllikewildcardescaper.h"

class QueryUtilTest : public MixxxTest {
  protected:
    QueryUtilTest()
          : m_trackCollection(config()) {
        // This test only needs a connection to an empty database
        // without any particular schema. No need to initialize the
        // database schema.
    }

    TrackCollection m_trackCollection;
};

TEST_F(QueryUtilTest, FieldEscaperEscapesQuotes) {
    FieldEscaper fieldEscaper(m_trackCollection.database());
    EXPECT_STREQ(qPrintable(QString("'foobar'")),
                 qPrintable(fieldEscaper.escapeString("foobar")));
    EXPECT_STREQ(qPrintable(QString("'foobar''s'")),
                 qPrintable(fieldEscaper.escapeString("foobar's")));
}

TEST_F(QueryUtilTest, SqlLikeWildcardEscaperEscapesForLike) {
    EXPECT_STREQ(qPrintable(QString("xx44xx4%yy4_yy")),
                 qPrintable(SqlLikeWildcardEscaper::apply("xx4xx%yy_yy", '4')));
}
