(:*******************************************************:)
(:Test: op-dateTime-greater-than-12                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-greater-than" function used  :)
(:together with "fn:true"/or expression (ge operator).   :)
(:*******************************************************:)
 
(xs:dateTime("1970-04-02T12:00:20Z") ge xs:dateTime("1980-04-02T12:00:20Z")) or (fn:true())