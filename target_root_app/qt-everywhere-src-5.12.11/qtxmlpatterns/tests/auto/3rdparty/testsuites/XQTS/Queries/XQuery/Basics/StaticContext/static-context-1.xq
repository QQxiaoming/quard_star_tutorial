(:*******************************************************:)
(:Test: static-context-1                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 12, 2006                                    :)
(:Purpose: Evaluate error condition XPST0001             :)
(:*******************************************************:)

declare namespace test = 'http://www.example.com'; 

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<a/> instance of element(*, test:unknownType)