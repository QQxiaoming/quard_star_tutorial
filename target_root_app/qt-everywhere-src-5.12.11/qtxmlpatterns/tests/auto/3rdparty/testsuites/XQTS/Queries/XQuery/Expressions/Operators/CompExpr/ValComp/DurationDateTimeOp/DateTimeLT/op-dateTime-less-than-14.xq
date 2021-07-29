(:*******************************************************:)
(:Test: op-dateTime-less-than-14                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-less-than" function used  :)
(:together with "fn:false"/or expression (le operator).  :)
(:*******************************************************:)
 
(xs:dateTime("1976-04-03T12:00:00Z") le xs:dateTime("2002-07-02T12:00:30Z")) or (fn:false())