(:*******************************************************:)
(: Test: default_namespace-021.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 03:59:40 2005                        :)
(: Purpose: Demonstrates Default namespace declaration,  :)
(:          facilitates the use of unprefixed QNames     :)
(:*******************************************************:)

declare default element namespace "http:/www.oracle.com/xquery";
declare variable $x := 7.5;
$x + 2
