(:*******************************************************:)
(: Test: K2-NodeLangFunc-1                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:33+01:00                       :)
(: Purpose: Invoke fn:lang from the top and bottom of a tree. :)
(:*******************************************************:)
let $i := <e xml:lang="en">
    <b xml:lang="de"/>
</e>
return (lang("de", exactly-one($i/b)), lang("de", $i))
