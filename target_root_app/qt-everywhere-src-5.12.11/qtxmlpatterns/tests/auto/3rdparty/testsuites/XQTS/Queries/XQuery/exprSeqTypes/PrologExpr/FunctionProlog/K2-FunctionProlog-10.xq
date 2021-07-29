(:*******************************************************:)
(: Test: K2-FunctionProlog-10                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A function whose name is only '_' and is declared in the default namespace. :)
(:*******************************************************:)
declare default function namespace "http://example.com";
declare function _()
{
    1
};
_()