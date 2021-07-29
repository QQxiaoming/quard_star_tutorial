(:*******************************************************:)
(: Test: K-SeqExprCast-1382                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:anyURI to xs:integer isn't allowed. :)
(:*******************************************************:)
xs:anyURI("http://www.example.com/an/arbitrary/URI.ext") cast as xs:integer