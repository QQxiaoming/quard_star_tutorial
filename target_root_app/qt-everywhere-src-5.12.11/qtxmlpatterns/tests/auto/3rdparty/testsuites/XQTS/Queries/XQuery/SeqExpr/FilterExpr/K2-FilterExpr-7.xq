(:*******************************************************:)
(: Test: K2-FilterExpr-7                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use self::processing-instruction() in a filter predicate. :)
(:*******************************************************:)
(<x/>, <?y?>)[self::processing-instruction()]