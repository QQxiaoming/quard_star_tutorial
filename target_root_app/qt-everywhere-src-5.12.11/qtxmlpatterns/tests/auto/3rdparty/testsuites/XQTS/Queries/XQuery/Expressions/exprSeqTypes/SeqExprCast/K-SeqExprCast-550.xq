(:*******************************************************:)
(: Test: K-SeqExprCast-550                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:decimal to xs:integer is allowed and should always succeed. :)
(:*******************************************************:)
xs:decimal("10.01") cast as xs:integer
                    ne
                  xs:integer("6789")