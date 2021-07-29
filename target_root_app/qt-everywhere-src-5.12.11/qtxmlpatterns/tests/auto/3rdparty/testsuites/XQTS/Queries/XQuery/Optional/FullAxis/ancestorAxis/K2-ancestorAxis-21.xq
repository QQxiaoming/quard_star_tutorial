(:*******************************************************:)
(: Test: K2-ancestorAxis-21                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use positional predicates.                   :)
(:*******************************************************:)
<r>
    <a>
        <b>
            <c/>
        </b>
    </a>
</r>/a/b/c/(ancestor::*[1], ancestor::*[2], ancestor::*[last()], ancestor::*[10])