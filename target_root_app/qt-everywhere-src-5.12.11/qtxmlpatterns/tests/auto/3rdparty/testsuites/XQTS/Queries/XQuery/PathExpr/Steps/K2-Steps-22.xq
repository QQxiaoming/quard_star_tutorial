(:*******************************************************:)
(: Test: K2-Steps-22                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Tricky combination of focuses, function calls, and a cardinality check. :)
(:*******************************************************:)
declare variable $root :=
<a>
    <b e="B"/>
    <c e="B"/>
</a>;
declare function local:function($arg)
{
    $root[exactly-one($arg/@e)]
};
$root/local:function(c)