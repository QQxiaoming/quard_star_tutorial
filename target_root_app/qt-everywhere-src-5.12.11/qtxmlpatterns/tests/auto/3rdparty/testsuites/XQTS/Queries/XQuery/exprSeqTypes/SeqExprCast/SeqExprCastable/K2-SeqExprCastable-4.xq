(:*******************************************************:)
(: Test: K2-SeqExprCastable-4                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Combine operator div with castable as(#2).   :)
(:*******************************************************:)
concat("2007-01-3", 1 div 0) castable as xs:date