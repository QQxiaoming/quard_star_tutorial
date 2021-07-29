(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-10                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure node identity is handled through several variables. :)
(:*******************************************************:)
declare variable $e := <e>
    <a/>
</e>;
declare variable $f := $e;
<r>
{
    $e is $e,
    $f is $e,
    $e, $f
}
</r>