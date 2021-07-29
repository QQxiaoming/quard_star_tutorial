(:*******************************************************:)
(: Test: K-SeqExprCast-922                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:date to xs:dateTime is allowed and should always succeed. :)
(:*******************************************************:)
xs:date("2004-10-13") cast as xs:dateTime
                    ne
                  xs:dateTime("2002-10-10T12:00:00-05:00")