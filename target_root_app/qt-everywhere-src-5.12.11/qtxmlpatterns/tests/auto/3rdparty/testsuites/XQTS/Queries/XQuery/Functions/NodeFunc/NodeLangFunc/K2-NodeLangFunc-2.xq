(:*******************************************************:)
(: Test: K2-NodeLangFunc-2                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Start navigating from the actual attribute node. :)
(:*******************************************************:)
let $i := <e xml:lang="en">
    <b xml:lang="de"/>
</e>
return lang("de", $i/b/@xml:lang)