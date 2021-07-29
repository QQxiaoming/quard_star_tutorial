(:*******************************************************:)
(: Test: K-SeqExprCast-824                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:dateTime to xs:gYearMonth is allowed and should always succeed. :)
(:*******************************************************:)
xs:dateTime("2002-10-10T12:00:00-05:00") cast as xs:gYearMonth
                    ne
                  xs:gYearMonth("1999-11")