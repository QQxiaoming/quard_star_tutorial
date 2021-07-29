(:*******************************************************:)
(: Test: K2-FunctionProlog-24                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The empty string cannot be cast to an xs:boolean. :)
(:*******************************************************:)
declare function local:distinct-nodes-stable ($arg as node()*) as xs:boolean* 
{ 
    for $a in $arg 
    return $a
};

local:distinct-nodes-stable((<element1/>,<element2/>))