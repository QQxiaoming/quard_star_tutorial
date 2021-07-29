(:*******************************************************:)
(: Test: K2-SeqExprCastable-8                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check castability of an invalid xs:anyURI.   :)
(:*******************************************************:)
("http:\\invalid>URI\someURI") castable as xs:anyURI