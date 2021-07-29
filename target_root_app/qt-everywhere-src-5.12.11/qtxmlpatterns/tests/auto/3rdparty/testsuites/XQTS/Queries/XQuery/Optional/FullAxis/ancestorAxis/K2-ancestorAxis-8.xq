(:*******************************************************:)
(: Test: K2-ancestorAxis-8                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply the ancestor axis to a simple tree constructed with constructors, combined with a [1]-predicate. :)
(:*******************************************************:)
<a>
    <b c="">
        <c/>
    </b>
    <d/>
</a>/b/c/ancestor::*[1]