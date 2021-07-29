(:*******************************************************:)
(: Test: K2-NodeLangFunc-1                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke fn:lang from the top and bottom of a tree. :)
(:*******************************************************:)
let $i := <e xml:lang="en">
    <b xml:lang="de"/>
</e>
return (lang("de", $i/b), lang("de", $i))