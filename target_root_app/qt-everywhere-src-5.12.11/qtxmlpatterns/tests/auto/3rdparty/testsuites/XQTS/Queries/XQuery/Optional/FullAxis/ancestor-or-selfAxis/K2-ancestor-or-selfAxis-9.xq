(:*******************************************************:)
(: Test: K2-ancestor-or-selfAxis-9                       :)
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
</r>/a/b/c/(ancestor-or-self::*[1], ancestor-or-self::*[2], ancestor-or-self::*[last()], ancestor-or-self::*[10])