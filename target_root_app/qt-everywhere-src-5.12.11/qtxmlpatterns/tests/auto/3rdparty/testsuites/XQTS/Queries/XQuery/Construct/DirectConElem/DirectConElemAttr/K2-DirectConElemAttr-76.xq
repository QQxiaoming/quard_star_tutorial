(:*******************************************************:)
(: Test: K2-DirectConElemAttr-76                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Pass text nodes through a function and variable, into AVTs. :)
(:*******************************************************:)
declare function local:t()
{
    text{""},
    text{"["},
    text{"3"},
    text{"]"},
    text{""}
};
declare variable $var := (text{""},
                          text{"["},
                          text{"3"},
                          text{"]"},
                          text{""});
<out fromFunction="{local:t()}" fromVariable="{$var}"/>