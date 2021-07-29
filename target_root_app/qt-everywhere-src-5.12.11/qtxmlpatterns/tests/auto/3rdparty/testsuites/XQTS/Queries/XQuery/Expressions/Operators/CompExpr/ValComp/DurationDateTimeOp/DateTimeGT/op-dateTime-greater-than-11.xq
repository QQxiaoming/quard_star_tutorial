(:*******************************************************:)
(:Test: op-dateTime-greater-than-11                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-greater-than" function used  :)
(:together with "fn:true"/or expression (gt operator).   :)
(:*******************************************************:)
 
(xs:dateTime("1990-04-02T12:00:10Z") gt xs:dateTime("2006-06-02T12:10:00Z")) or (fn:true())