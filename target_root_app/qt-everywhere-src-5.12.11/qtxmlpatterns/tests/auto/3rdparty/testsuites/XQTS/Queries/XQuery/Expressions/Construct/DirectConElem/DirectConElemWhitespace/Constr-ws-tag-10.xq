(: Name: Constr-ws-tag-10 :)
(: Written by: Andreas Behm :)
(: Description: count text nodes when preserving whitespace :)

declare boundary-space preserve;
fn:count((<elem>   	
  	        <a>       	            
		

   	     </a>
<a>     	        </a>    	         <a>	  <b>

      	    </b>

  </a>
	

      </elem>)//text())