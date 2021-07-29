(:*******************************************************:)
(: Test: K2-DocumentURIFunc-9                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke on a tree document node.              :)
(:*******************************************************:)
let $i := document
{
<e>
        <a/>
        <a/>
        <a/>
            <b/>
            <b/>
        <a/>
        <a/>
</e>
}
return empty(document-uri($i))