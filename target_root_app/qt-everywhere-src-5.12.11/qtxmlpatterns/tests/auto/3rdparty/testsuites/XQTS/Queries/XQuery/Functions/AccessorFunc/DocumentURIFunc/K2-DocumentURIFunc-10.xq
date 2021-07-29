(:*******************************************************:)
(: Test: K2-DocumentURIFunc-10                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke on a tree document node with fn:root(). :)
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
return empty(document-uri(root(($i/a/b)[1])))