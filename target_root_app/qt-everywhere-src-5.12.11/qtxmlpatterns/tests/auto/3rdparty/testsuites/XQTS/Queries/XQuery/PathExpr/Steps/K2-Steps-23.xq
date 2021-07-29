(:*******************************************************:)
(: Test: K2-Steps-23                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Tricky combination of focuses and function calls(#2). :)
(:*******************************************************:)
declare variable $root := <a><c e=""/></a>;
declare function local:function($arg)
{
    $root[$arg/@e]
};
$root/local:function(c)