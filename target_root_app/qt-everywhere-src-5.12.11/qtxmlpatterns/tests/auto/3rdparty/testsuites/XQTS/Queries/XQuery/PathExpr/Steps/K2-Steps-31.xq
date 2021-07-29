(:*******************************************************:)
(: Test: K2-Steps-31                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Tricky combination of focuses and function calls(#9). :)
(:*******************************************************:)
declare variable $root := <root><c/></root>;

declare function local:function($arg)
{
    $root[$arg]
};
    
$root//local:function(.)