(:*******************************************************:)
(: Test: K2-Steps-34                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Tricky combination of focus and a recursive function call(#3). :)
(:*******************************************************:)
declare variable $root :=
(
    <b d=""/>,
    <c d="">
        <c d=""/>
    </c>
);

declare function local:function($object)
{
    $root[@d eq $object/@d]
};
$root/local:function(c)