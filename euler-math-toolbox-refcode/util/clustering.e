// Clustering Data

// Load this file with "load clustering".

comment
Functions for clustering data.
endcomment

function kmeanscluster (x : numerical, k : index)
## Cluster the rows in x in k clusters
##
## It uses the algorithm proposed by Lloyd, and used by Steinhaus,
## MacQueen. The algorithm starts with a random partition (cluster).
## Then it computes the means of the clusters, and associates each
## point to the cluster with the closest mean. It loops this
## procedure until there are no changes.
##
## The function works for multi-dimensional x too. The means are then
## vector means, and the distance to the mean is measured in Euclidean
## distance.
##
## x : rows containing the data points
## k : number of clusters that should be used
##
## Returns j : indices of the clusters the rows should belong to.
	n=rows(x); m=cols(x);
	i=floor((0:k)/k*(n-1))+1;
	means=zeros(k,m);
	loop 1 to k;
		means[#]=sum(x[i[#]:(i[#+1]-1)]')'/(i[#+1]-i[#]);
	end;
	j=1:n;
	loop 1 to n;
		d=sum((x[#]-means)^2);
		j[#]=extrema(d')[2];
	end;
	repeat
		loop 1 to k;
			i=nonzeros(j==#);
			if cols(i)==0 then means[#]=1;
			else means[#]=(sum(x[i]')/cols(i))';
			endif;
		end;
		jold=j;
		loop 1 to n;
			d=sum((x[#]-means)^2);
			j[#]=extrema(d')[2];
		end;
		if all(jold==j) then break; endif;
	end
	return j
endfunction

function similaritycluster (S : numerical, k : index)
## Cluster data depending on the similarity matrix S
##
## This clustering uses the first k eigenvalue of S, and clusters
## the entries of their eigenvalues.
##
## S : similarity matrix (symmetric)
## k : number of clusters
##
## Returns j : indices of the clusters the rows should belong to.
	lambda=sort(jacobi(S));
	v=zeros(cols(S),0);
	loop 1 to k
		v=v|eigenspace(S,lambda[#]);
	end
	return kmeanscluster(v,k);
endfunction

function eigencluster (x : numerical, k : index)
## Cluster the rows in x in k clusters
##
## This algorithm uses the similarity matrix S, which contains the
## Euclidean distances of two rows in x. Then it uses the the function
## similaritycluster() to get the clustering of the similarity
## matrix.
##
## x : rows containing the data points
## k : number of clusters that should be used
##
## Returns j : indices of the clusters the rows should belong to.
	P=x';
	S=(P[1]-P[1]')^2;
	loop 2 to cols(x)
		S=S+(P[#]-P[#]')^2;
	end;
	S=sqrt(S); D=sqrt(sum(S));
	S=exp(-((S/D)/D')^2); D=sqrt(sum(S));
	S=id(cols(S))-(S/D)/D';
	return similaritycluster(S,k);
endfunction

addmenu kmeanscluster(?matrixWithDataRows,?numberOfClusters)
addmenu similaritycluster(?laplaceMatrix,?numberOfClusters)
addmenu eigencluster(?matrixWithDataRows,?numberOfClusters)
