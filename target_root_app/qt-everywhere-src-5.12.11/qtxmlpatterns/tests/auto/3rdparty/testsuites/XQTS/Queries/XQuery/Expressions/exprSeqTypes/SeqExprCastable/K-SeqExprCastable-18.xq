(:*******************************************************:)
(: Test: K-SeqExprCastable-18                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: When casting to xs:QName the source value can be a xs:QName value. :)
(:*******************************************************:)
QName("", "lname") castable as xs:QName