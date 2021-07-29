(:*******************************************************:)
(: Test: K2-ForExprWithout-30                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a name test named 'ordered' inside a for loop, inside a user function. :)
(:*******************************************************:)
declare function local:func($arg as element()* ) as element()*
{
    for $n in $arg/ordered return $n
};
1