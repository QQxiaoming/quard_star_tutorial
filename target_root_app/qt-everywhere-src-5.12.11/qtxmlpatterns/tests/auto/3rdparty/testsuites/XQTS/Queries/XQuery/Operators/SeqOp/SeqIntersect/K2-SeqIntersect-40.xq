(:*******************************************************:)
(: Test: K2-SeqIntersect-40                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Explicitly sort the result of an intersect expression. :)
(:*******************************************************:)
(: insert-start :)
    declare variable $input-context1 external;
    (: insert-end :)
    <r>
        {
            $input-context1//(employee[location = "Denver"] intersect $input-context1//employee[last()])/./location
        }
    </r>
