(:*******************************************************:) 
(: Test: fn-deep-equal-node-args-3.xq                    :) 
(: Written By: Andrew Eisenberg                          :) 
(: Date: 2006-09-25                                      :) 
(: Purpose: Apply deep-equal to nodes                    :) 
(:*******************************************************:) 
 
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:deep-equal(<a> {$input-context/node(), $input-context/node()} </a>/node(),
              <b> {$input-context/node(), 'difference', $input-context/node()} </b>/node()
) 
