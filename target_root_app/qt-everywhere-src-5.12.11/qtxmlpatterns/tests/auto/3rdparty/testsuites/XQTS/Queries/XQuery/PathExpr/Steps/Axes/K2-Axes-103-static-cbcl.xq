(:*******************************************************:)
(: Test: K2-Axes-103                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:32+01:00                       :)
(: Purpose: Combine a function call with paths that requires sorting. :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

declare function local:isComplexType($typeID as item()?)
{
    string($typeID)
};
"|", $input-context1//*/local:isComplexType(@type), "|"
