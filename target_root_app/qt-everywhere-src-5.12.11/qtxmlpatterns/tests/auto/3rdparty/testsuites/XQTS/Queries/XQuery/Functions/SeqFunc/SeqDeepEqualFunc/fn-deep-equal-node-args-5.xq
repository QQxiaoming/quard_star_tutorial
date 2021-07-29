(:*******************************************************:) 
(: Test: fn-deep-equal-node-args-5.xq                    :) 
(: Written By: Andrew Eisenberg                          :) 
(: Date: 2006-09-25                                      :) 
(: Purpose: Apply deep-equal to nodes                    :) 
(:*******************************************************:) 
 
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:deep-equal(<a> {$input-context/node(), <diff x='1'/>, $input-context/node()} </a>/node(),
              <b> {$input-context/node(), <diff xx='1'/>, $input-context/node()} </b>/node()
) 
