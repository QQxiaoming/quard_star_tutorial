(:*******************************************************:)
(: Test: function-declaration-025.xq                     :)
(: Written By: Carmelo Montanez                          :)
(: Date: February 6, 2006                                :)
(: Purpose: Function Declaration with no namespace.      :)
(:*******************************************************:)
declare default function namespace "";

declare function foo ($n as xs:integer, $m as xs:integer)
{
 $n
};
foo(4, 1)