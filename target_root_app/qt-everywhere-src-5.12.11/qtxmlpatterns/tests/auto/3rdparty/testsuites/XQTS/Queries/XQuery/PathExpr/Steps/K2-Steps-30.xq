(:*******************************************************:)
(: Test: K2-Steps-30                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Tricky combination of focuses and function calls(#8). :)
(:*******************************************************:)
declare variable $root :=
<root>
        <c d=""/>
</root>;

declare function local:function($object)
{
    $root[$object]
};
    
$root//local:function(c)