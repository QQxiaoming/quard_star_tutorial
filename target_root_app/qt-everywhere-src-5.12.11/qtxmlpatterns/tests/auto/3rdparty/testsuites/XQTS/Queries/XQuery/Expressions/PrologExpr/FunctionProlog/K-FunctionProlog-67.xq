(:*******************************************************:)
(: Test: K-FunctionProlog-67                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A function call that reminds of the range expression. :)
(:*******************************************************:)

declare default function namespace "http://www.w3.org/2005/xquery-local-functions";
declare function local:is() as xs:integer
{
	1
};
is() eq 1
