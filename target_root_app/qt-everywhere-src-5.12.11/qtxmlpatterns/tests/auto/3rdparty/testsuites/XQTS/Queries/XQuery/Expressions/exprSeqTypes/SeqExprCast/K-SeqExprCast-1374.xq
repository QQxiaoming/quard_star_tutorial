(:*******************************************************:)
(: Test: K-SeqExprCast-1374                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:anyURI to xs:string is allowed and should always succeed. :)
(:*******************************************************:)
xs:anyURI("http://www.example.com/an/arbitrary/URI.ext") cast as xs:string
                    ne
                  xs:string("an arbitrary string")