(:*******************************************************:)
(: Test: K2-NodeLangFunc-2                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:33+01:00                       :)
(: Purpose: Start navigating from the actual attribute node. :)
(:*******************************************************:)
let $i := <e xml:lang="en">
    <b xml:lang="de"/>
</e>
return lang("de", exactly-one($i/b/@xml:lang))
