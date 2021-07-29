(:*******************************************************:)
(: Test: K2-Steps-32                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Tricky combination of focus and a recursive function call. :)
(:*******************************************************:)
declare variable $root := <root/>;
declare function local:function($arg, $count as xs:integer)
{
    $arg,
    $root,
    if($count eq 2)
    then $root
    else local:function($arg, $count + 1)
};
$root/local:function(., 1)
