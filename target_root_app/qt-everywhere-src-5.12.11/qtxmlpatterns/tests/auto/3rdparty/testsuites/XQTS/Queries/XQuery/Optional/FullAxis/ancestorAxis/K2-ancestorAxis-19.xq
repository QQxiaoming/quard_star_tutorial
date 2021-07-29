(:*******************************************************:)
(: Test: K2-ancestorAxis-19                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply fn:count() to the result of axis ancestor. :)
(:*******************************************************:)
count(<a>
    <b c="">
        <c/>
    </b>
    <d/>
</a>/b/c/(ancestor::*))