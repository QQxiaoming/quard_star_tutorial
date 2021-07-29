(:*******************************************************:) 
(: Test: fn-deep-equal-node-args-2.xq                    :) 
(: Written By: Andrew Eisenberg                          :) 
(: Date: 2006-09-25                                      :) 
(: Purpose: Apply deep-equal to nodes                    :) 
(:*******************************************************:) 
 
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:deep-equal(<a> {$input-context/bib/node(), $input-context/bib/node()} </a>/node(),
              <b> {$input-context/bib/node(), <difference/>, $input-context/bib/node()} </b>/node()
             ) 
