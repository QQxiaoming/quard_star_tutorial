(:*******************************************************:)
(: Test: K-SeqExprCast-818                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:dateTime to xs:dateTime is allowed and should always succeed. :)
(:*******************************************************:)
xs:dateTime("2002-10-10T12:00:00-05:00") cast as xs:dateTime
                    eq
                  xs:dateTime("2002-10-10T12:00:00-05:00")