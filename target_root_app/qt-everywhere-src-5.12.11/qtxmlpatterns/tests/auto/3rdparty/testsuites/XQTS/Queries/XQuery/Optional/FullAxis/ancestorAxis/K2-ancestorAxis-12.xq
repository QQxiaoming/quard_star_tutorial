(:*******************************************************:)
(: Test: K2-ancestorAxis-12                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply the ancestor axis to a simple tree constructed with constructors, combined with fn:last(). Paranteses are added to ensure that the result of the axis step is delivered in document order. :)
(:*******************************************************:)
<a>
    <b c="">
        <c/>
    </b>
    <d/>
</a>/b/c/(ancestor::*)[last()]