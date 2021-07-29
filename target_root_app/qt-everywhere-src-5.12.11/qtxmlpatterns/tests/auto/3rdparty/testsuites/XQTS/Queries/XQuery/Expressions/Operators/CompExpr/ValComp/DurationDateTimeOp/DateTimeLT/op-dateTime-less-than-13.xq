(:*******************************************************:)
(:Test: op-dateTime-less-than-13                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-less-than" function used  :)
(:together with "fn:false"/or expression (lt operator).  :)
(:*******************************************************:)
 
(xs:dateTime("1981-04-02T12:00:00Z") lt xs:dateTime("2003-04-02T12:10:00Z")) or (fn:false())