(:*******************************************************:)
(: Test: K2-ForExprWithout-36                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a name test named 'validate' inside a for loop, inside a user function. :)
(:*******************************************************:)
declare function local:func($arg as element()* ) as element()*
{
    for $n in $arg/validate return $n
};
1