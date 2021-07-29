(:*******************************************************:)
(:Test: op-dateTime-less-than-12                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-less-than" function used  :)
(:together with "fn:true"/or expression (le operator).   :)
(:*******************************************************:)
 
(xs:dateTime("1970-04-02T12:00:20Z") le xs:dateTime("1980-04-02T12:00:20Z")) or (fn:true())