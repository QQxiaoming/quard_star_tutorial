(:*******************************************************:)
(: Test: K2-Axes-57                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The context item invokes sorting and duplicate elimination. :)
(:*******************************************************:)
declare variable $var := <a>
    <b/>
    <c/>
    <d/>
</a>;
($var/*, $var/*)/.