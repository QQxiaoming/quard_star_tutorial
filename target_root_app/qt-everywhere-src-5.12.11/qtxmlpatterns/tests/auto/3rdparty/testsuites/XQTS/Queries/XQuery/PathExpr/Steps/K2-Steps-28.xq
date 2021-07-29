(:*******************************************************:)
(: Test: K2-Steps-28                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Tricky combination of focuses and function calls(#6). :)
(:*******************************************************:)
declare variable $root :=
<root>
    <b d=""/>
    <c d=""/>
</root>;
    
declare function local:function($object)
{
    $root[$object/@d]
};
$root//local:function(c)
