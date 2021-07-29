(:*******************************************************:)
(: Test: K2-SeqIDFunc-13                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use on xml:id with input strings which contains many IDREFs, and invalid ones too. :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)
fn:id($input-context1//b/@ref, $input-context1)/data(exactly-one(@*))
    
                            