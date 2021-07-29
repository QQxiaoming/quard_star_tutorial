(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-12                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Reference a variable from two different node constructors. :)
(:*******************************************************:)
declare variable $a as attribute()* := (attribute name1 {()}, attribute name2 {()}, attribute name3 {()});
<r>
    <e>
        {
            $a
        }
    </e>
    <e>
        {
            $a
        }
    </e>
</r>