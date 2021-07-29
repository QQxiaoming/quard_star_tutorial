(:*******************************************************:)
(: Test: K2-Axes-75                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Combine axis descendant-or-self, function last() and axis ancestor. :)
(:*******************************************************:)
<a>
    <b c=""/>
    <d/>
</a>/descendant-or-self::node()[last()]/ancestor::*