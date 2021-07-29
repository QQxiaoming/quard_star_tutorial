(:*******************************************************:)
(: Test: K2-FilterExpr-2                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply a predicate combined with a numeric literal, on a sequence constructed with an element constructor. :)
(:*******************************************************:)
declare variable $var := (for $i in 1 to 100 return <e>{$i}</e>);
                  $var[5]