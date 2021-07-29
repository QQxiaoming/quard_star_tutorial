(:*******************************************************:)
(: Test: K2-ancestor-or-selfAxis-7                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply fn:count() to the result of axis ancestor-or-self. :)
(:*******************************************************:)
count(<a>
    <b c="">
        <c/>
    </b>
    <d/>
</a>/b/c/(ancestor-or-self::*))