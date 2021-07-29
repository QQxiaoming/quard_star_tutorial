(:*******************************************************:)
(: Test: K2-Axes-58                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Content being duplicated with the comma operator.. :)
(:*******************************************************:)
declare variable $var := <a>
    <b/>
    <c/>
    <d/>
</a>;
($var/*, $var/*)