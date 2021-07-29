(:*******************************************************:)
(: Test: function-declaration-026.xq                     :)
(: Written By: Frans Englich                             :)
(: Date: October 20, 2008                                :)
(: Purpose: Function Declaration using global variable.  :)
(:*******************************************************:)
declare variable $global := node(); (: A global node test, which uses the focus in the dynamic context. :)
declare function local:function()
{
    exists($global)
};
local:function()
