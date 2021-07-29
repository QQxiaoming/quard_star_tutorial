(:*******************************************************:)
(: Test: K2-ComputeConElem-15                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use the default element, where the name is computed dynamically. :)
(:*******************************************************:)
declare default element namespace "http://example.com/NS";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)
    
element {exactly-one(($input-context1//*)[3])} {}