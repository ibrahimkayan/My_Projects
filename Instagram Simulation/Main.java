import java.io.BufferedReader;   // Used for efficient reading of text from the input file.
import java.io.FileReader;       // Allows reading character files.
import java.io.FileWriter;       // Enables writing character files.
import java.io.IOException;      // Handles exceptions related to input/output operations.
import java.util.ArrayList;      // Provides support for dynamic arrays.

// Main class
public class Main {
    private HashTable<String, User> userTable; // HashTable to store user objects, keyed by user ID.
    private HashTable<String, Post> postTable; // HashTable to store post objects, keyed by post ID.

    // Constructor initializes the hash tables with default capacity.
    public Main() {
        userTable = new HashTable<String, User>(16); // User hash table with initial size 16.
        postTable = new HashTable<String, Post>(16); // Post hash table with initial size 16.
    }

    public static void main(String[] args) {

        Main myMainMethod = new Main(); // Instantiate the Main class to access its methods.
        String inputFilePath = args[0]; // Path to the input file.
        String outputFilePath = args[1]; // Path to the output file.


        // Use try-with-resources to ensure the reader and writer are closed automatically.
        try (BufferedReader reader = new BufferedReader(new FileReader(inputFilePath));
             FileWriter writer = new FileWriter(outputFilePath)) {

            String line; // Variable to hold each line from the input file.
            // Read each line from the input file.
            while ((line = reader.readLine()) != null) {
                // Split the line into parts to extract the command and arguments.
                String[] parts = line.split(" ");
                String command = parts[0]; // The first part is always the command

                // Switch-case to handle various commands from the input file.
                switch (command) {
                    case "create_user":
                        String userId = parts[1]; // User ID to be created.
                        writer.write(myMainMethod.createUser(userId) + "\n"); // Write the result to the output file.
                        break;

                    case "follow_user":
                        String user1Id = parts[1]; // Follower's user ID.
                        String user2Id = parts[2]; // User ID to be followed.
                        writer.write(myMainMethod.followUser(user1Id, user2Id) + "\n");
                        break;

                    case "unfollow_user":
                        String user1id = parts[1]; // Unfollower's user ID.
                        String user2id = parts[2]; // User ID to be unfollowed.
                        writer.write(myMainMethod.unfollowUser(user1id, user2id) + "\n");
                        break;

                    case "create_post":
                        String authorId = parts[1]; // Author of the post.
                        String postId = parts[2]; // Unique post ID.
                        String content = parts[3]; // Post content.
                        writer.write(myMainMethod.createPost(authorId, postId, content) + "\n");
                        break;

                    case "see_post":
                        String userId_see = parts[1]; // User viewing the post.
                        String postId_see = parts[2]; // Post being viewed.
                        writer.write(myMainMethod.seePost(userId_see, postId_see) + "\n");
                        break;

                    case "see_all_posts_from_user":
                        String userId_see_all = parts[1]; // User viewing posts.
                        String postId_see_all = parts[2]; // Author of the posts being viewed.
                        writer.write(myMainMethod.seeAllPost(userId_see_all, postId_see_all) + "\n");
                        break;

                    case "toggle_like":
                        String userId_like = parts[1]; // User liking/unliking a post.
                        String postId_like = parts[2]; // Post being liked/unliked.
                        writer.write(myMainMethod.toggleLike(userId_like, postId_like) + "\n");
                        break;

                    case "generate_feed":
                        String userId_feed = parts[1]; // User generating the feed.
                        String num_feed = parts[2]; // Number of posts to include in the feed.
                        writer.write(myMainMethod.generateFeed(userId_feed, num_feed) + "\n");
                        break;

                    case "scroll_through_feed":
                        String[] likes = new String[Integer.parseInt(parts[2])]; // Array to store like actions for scrolling.
                        System.arraycopy(parts, 3, likes, 0, likes.length); // Extract like actions from the input.
                        writer.write(myMainMethod.scrollThroughFeed(parts[1], parts[2], likes) + "\n");
                        break;

                    case "sort_posts":
                        String userIdSort = parts[1]; // User whose posts need to be sorted.
                        String resultSort = myMainMethod.Sort(userIdSort); // Sort the user's posts by likes.
                        writer.write(resultSort + "\n");
                        break;

                    default:
                        writer.write("Invalid command\n");
                }
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /// Represents a user in a social media platform, managing their connections and interactions.
    class User {
        String id;                      // Unique identifier for the user
        ArrayList<User> followings;     // List of users this user is following
        ArrayList<User> followers;      // List of users following this user
        ArrayList<Post> sharedPosts;    // Posts shared by this user
        ArrayList<Post> likedPosts;     // Posts liked by this user
        ArrayList<Post> viewedPosts;    // Posts viewed by this user

        /// Constructor to initialize a user with a given id.
        public User(String id) {
            this.id = id;
            this.followings = new ArrayList<>();
            this.followers = new ArrayList<>();
            this.sharedPosts = new ArrayList<>();
            this.likedPosts = new ArrayList<>();
            this.viewedPosts = new ArrayList<>();
        }

        /// Adds a user to the following list, returns true if successful.
        public boolean addFollowing(User following) {
            if (followings.contains(following)) {
                return false; // Already following
            }
            followings.add(following);
            return true;
        }

        /// Removes a user from the following list, returns true if successful.
        public boolean removeFollowing(User following) {
            return followings.remove(following);
        }

        /// Adds a user to the follower list, returns true if successful.
        public boolean addFollower(User follower) {
            if (followers.contains(follower)) {
                return false; // Already a follower
            }
            followers.add(follower);
            return true;
        }

        /// Removes a user from the follower list, returns true if successful.
        public boolean removeFollower(User follower) {
            return followers.remove(follower);
        }
    }


    /// Represents a post shared by a user on the platform.
    class Post {
        String id;           // Unique identifier for the post
        String authorId;     // ID of the user who authored the post
        String content;      // Content of the post
        int likeCount;       // Number of likes on the post

        /// Constructor to create a post with a unique ID, author ID, and content.
        public Post(String postId, String authorId, String content) {
            this.id = postId;
            this.authorId = authorId;
            this.content = content;
            this.likeCount = 0; // Initialize like count to 0
        }
    }

    // Creates a new user with the given userId.
    // If a user with the same userId already exists, an error message is returned.
    public String createUser(String userId) {
        if (userTable.get(userId) != null) { // Check if the user already exists.
            return "Some error occurred in create_user.";
        }
        User user = new User(userId); // Create a new User object.
        userTable.put(userId, user); // Add the user to the userTable.
        return "Created user with Id " + userId + ".";
    }

    // Allows one user (follower) to follow another user (following).
    // If the follower or following user does not exist, or if they are the same, or the relationship already exists, an error message is returned.
    public String followUser(String followerId, String followingId) {
        User follower = userTable.get(followerId); // Retrieve the follower.
        User following = userTable.get(followingId); // Retrieve the user to be followed.

        if (follower == following) { // Prevent users from following themselves.
            return "Some error occurred in follow_user.";
        }

        if (follower == null || following == null) { // Check if either user does not exist.
            return "Some error occurred in follow_user.";
        }

        if (!follower.addFollowing(following)) { // Add the following relationship; check for errors.
            return "Some error occurred in follow_user.";
        }
        following.addFollower(follower); // Add the follower relationship.
        return followerId + " followed " + followingId + ".";
    }

    // Allows one user (unfollower) to unfollow another user (unfollowed).
    // If the unfollower or unfollowed does not exist, or they are the same, or the relationship does not exist, an error message is returned.
    public String unfollowUser(String unfollowerId, String unfollowingId) {
        User unfollower = userTable.get(unfollowerId);// Retrieve the unfollower.
        User unfollowee = userTable.get(unfollowingId);// Retrieve the user to be unfollowed.

        // Prevent users from unfollowing themselves.
        if (unfollower==unfollowee){return "Some error occurred in unfollow_user.";}

        if (unfollower == null || unfollowee == null) {// Check if either user does not exist.
            return "Some error occurred in unfollow_user.";
        }

        if (!unfollower.removeFollowing(unfollowee)) {// Remove the following relationship; check for errors.
            return "Some error occurred in unfollow_user.";
        }
        unfollowee.removeFollower(unfollower);// Remove the follower relationship.
        return unfollowerId + " unfollowed " + unfollowingId + ".";
    }

    // Creates a post authored by the user with the given content and postId.
    // If the user does not exist or the postId is already in use, an error message is returned.
    public String createPost(String authorId, String postId, String content) {
        User author = userTable.get(authorId);// Retrieve the author of the post.
        if (author == null) { // Check if the author exists.
            return "Some error occurred in create_post.";
        }

        if (postTable.get(postId) != null) { // Check if the postId is already used.
            return "Some error occurred in create_post.";
        }

        Post newPost = new Post(postId, authorId, content);// Create a new Post object.
        postTable.put(postId,newPost);// Add the post to the postTable.
        author.sharedPosts.add(newPost); // Add the post to the author's list of shared posts.

        return authorId + " created a post with Id " + postId + ".";
    }
    // Records that a user has viewed a specific post.
    // If the user or post does not exist, an error message is returned.
    public String seePost(String userId, String postId) {
        User viewer = userTable.get(userId);// Retrieve the user viewing the post.
        if (viewer == null) {// Check if the user exists.
            return "Some error occurred in see_post.";
        }

        Post post = postTable.get(postId);// Retrieve the post.
        if (post == null) {// Check if the post exists.
            return "Some error occurred in see_post.";
        }

        viewer.viewedPosts.add(post);// Add the post to the user's list of viewed posts.

        return userId + " saw " + postId + ".";
    }

    // Records that a user has viewed all posts shared by another user.
    // If either user does not exist, an error message is returned.
    public String seeAllPost(String viewerId, String viewedId) {
        User viewer = userTable.get(viewerId);// Retrieve the viewer.
        User viewed = userTable.get(viewedId);// Retrieve the user whose posts are being viewed.

        if (viewer == null || viewed == null) {// Check if both users exist.
            return "Some error occurred in see_all_posts_from_user.";
        }

        for (Post post : viewed.sharedPosts) {// Iterate through the shared posts.
            if (!viewer.viewedPosts.contains(post)) {// Avoid adding duplicates.
                viewer.viewedPosts.add(post);
            }
        }

        return viewerId + " saw all posts of " + viewedId + ".";
    }
    // Toggles a like for a post by a user: likes the post if not already liked, or unlikes it otherwise.
    // If the user or post does not exist, an error message is returned.
    public String toggleLike(String userId, String postId) {
        User user = userTable.get(userId);// Retrieve the user liking/unliking the post.
        if (user == null) {
            return "Some error occurred in toggle_like.";
        }

        Post post = postTable.get(postId);// Retrieve the post.
        if (post == null) {
            return "Some error occurred in toggle_like.";
        }

        if (!user.viewedPosts.contains(post)){// Ensure the post is marked as viewed.
            user.viewedPosts.add(post);
        }

        if (!user.likedPosts.contains(post)) {// Like the post if not already liked.
            user.likedPosts.add(post);
            post.likeCount++;
            return userId + " liked " + post.id + ".";
        } else {// Unlike the post if already liked.
            user.likedPosts.remove(post);
            post.likeCount--;
            return userId + " unliked " + post.id + ".";
        }
    }

    // Generates a personalized feed of posts for a user, based on the users they follow.
    // Returns up to `numStr` posts that the user has not yet viewed, prioritized by like count(and secondary lexicographically).
    public String generateFeed(String userId, String numStr) {
        User user = userTable.get(userId);// Retrieve the user.
        if (user == null) {// Check if the user exists.
            return "Some error occurred in generate_feed.";
        }

        int num = Integer.parseInt(numStr);// Parse the number of posts to retrieve.
        MaxHeap tempHeap = new MaxHeap(16);// Temporary heap to store posts.

        // Collect posts from followings into the heap if not already viewed.
        // Insert the posts to a temporary  max heap
        if (user.followings.size() != 0) {
            for (User followedUser : user.followings) {
                if (followedUser.sharedPosts.size() != 0) {
                    for (Post post_ : followedUser.sharedPosts) {
                        if (!user.viewedPosts.contains(post_)) {
                            tempHeap.insert(post_);
                        }
                    }
                }
            }
        }


        ArrayList<Post> feed = new ArrayList<>();
        while (feed.size() < num && !tempHeap.isEmpty()) {// Extract posts from the heap until the feed is full.
            Post post = tempHeap.removeMax();
            feed.add(post);
        }

        StringBuilder result = new StringBuilder("Feed for " + userId + ":\n");
        for (Post post : feed) {// Format the feed for output.
            if (post != null) {
                result.append("Post ID: ").append(post.id)
                        .append(", Author: ").append(post.authorId)
                        .append(", Likes: ").append(post.likeCount)
                        .append("\n");
            }
        }

        if (feed.size() < num) {// If fewer posts are available than requested, add a note.
            result.append("No more posts available for ").append(userId + ".");
        }

        if (result.length() > 0 && result.charAt(result.length() - 1) == '\n') {
            result.setLength(result.length() - 1);
        }

        return result.toString();
    }

    // Scrolling through the feed for a user.
    // Allows the user to view posts and optionally like them based on input.
    public String scrollThroughFeed(String userId, String numStr, String... likes) {
        User user = userTable.get(userId);// Retrieve the user.
        if (user == null) {// Check if the user exists.
            return "Some error occurred in scroll_through_feed.";
        }
        int num = Integer.parseInt(numStr);// Parse the number of posts to view.
        String result = userId + " is scrolling through feed:";
        // Collect posts from followings into the heap if not already viewed.
        // Insert the posts to a temporary  max heap
        MaxHeap postHeap = new MaxHeap(16);// Temporary heap to store posts.
        if (user.followings.size() != 0) {
            for (User followedUser : user.followings) {
                if (followedUser.sharedPosts.size() != 0) {
                    for (Post post_ : followedUser.sharedPosts) {
                        if (!user.viewedPosts.contains(post_)) {
                            postHeap.insert(post_);
                        }
                    }
                }
            }
        }

        // Scrolling and handle user interactions (like or just view).
        for (int i = 0; i < num; i++) {
            if (postHeap.isEmpty()) {// If no more posts are available
                result += "\nNo more posts in feed.";
                break;
            }

            Post post = postHeap.removeMax();// Get the next post.

            boolean liked = likes[i].equals("1");// Check if the user liked this post.
            if (liked) {
                user.likedPosts.add(post);// Add the post to the user's liked posts.
                post.likeCount++;
                result += "\n" + userId + " saw " + post.id + " while scrolling and clicked the like button.";
            } else {
                result += "\n" + userId + " saw " + post.id + " while scrolling.";
            }

            user.viewedPosts.add(post); // Mark the post as viewed.
        }
        return result;
    }

    // Sorts and displays a user's shared posts based on the number of likes (most to least).
    public String Sort(String userId) {
        User user = userTable.get(userId);// Retrieve the user.
        if (user == null) {// Check if the user exists.
            return "Some error occurred in sort_posts.";
        }

        if (user.sharedPosts.isEmpty()) {// Check if the user has shared any posts.
            return "No posts from " + userId + ".";
        }

        StringBuilder result = new StringBuilder("Sorting " + userId + "'s posts:");
        MaxHeap heapForSort = new MaxHeap(16);// Temporary heap to store posts.
        // Add all shared posts to the heap.
        for (Post post : user.sharedPosts) {
            if (post != null) {
                heapForSort.insert(post);
            }
        }
        // Retrieve posts from the heap in descending order of likes.
        while (!heapForSort.isEmpty()) {
            Post post = heapForSort.removeMax();
            result.append("\n").append(post.id).append(", Likes: ").append(post.likeCount);
        }
        return result.toString();
    }

    // A HashTable implementation with generic Key (K) and Value (V) types.
    // Uses separate chaining (open hashing) to resolve collisions and supports dynamic resizing.
    class HashTable<K, V> { // K: Key, V: Value
        private hashNode<K, V>[] table;// Array of hash nodes
        private int size;
        private final double loadFactor = 0.70; // // Maximum load factor before resizing

        public HashTable(int initialCapacity) {
            table = new hashNode[initialCapacity];
            size = 0;
        }

        // Inserts a key-value pair into the hash table, updating the value if the key exists.
        public void put(K key, V value) {
            int index = hash(key);
            hashNode<K, V> current = table[index];

            if (current == null) {
                table[index] = new hashNode<>(key, value);// Insert at the index if empty
                size++;
            } else {
                while (true) {
                    if (current.key.equals(key)) {
                        current.value = value;// Update the value if key exists
                        return;
                    }
                    if (current.next == null) {
                        current.next = new hashNode<>(key, value); // Add new node at the end
                        size++;
                        break;
                    }
                    current = current.next;
                }
            }

            // Rehashing
            if ((double) size / table.length > loadFactor) {
                rehash();
            }
        }

        // Retrieves the value associated with the specified key, or null if not found.
        public V get(K key) {
            int index = hash(key);
            hashNode<K, V> current = table[index];
            // Traverse the linked list to find the key
            while (current != null) {
                if (current.key.equals(key)) {
                    return current.value;
                }
                current = current.next;
            }
            return null; // If key not found
        }
        // Computes the hash index for a given key.
        private int hash(K key) {
            return Math.abs(key.hashCode()) % table.length;
        }
        // Resizes the hash table when the load factor exceeds the threshold.
        private void rehash() {
            hashNode<K, V>[] oldTable = table;
            table = new hashNode[oldTable.length * 2];
            size = 0;

            // Reinsert all nodes into the new table
            for (hashNode<K, V> node : oldTable) {
                while (node != null) {
                    put(node.key, node.value);
                    node = node.next;
                }
            }
        }
        // Nested class representing a single node in the hash table (for separate chaining).
        static class hashNode<K, V> {
            K key;
            V value;
            hashNode<K, V> next;

            public hashNode(K key, V value) {
                this.key = key;
                this.value = value;
                this.next = null;
            }
        }
    }

    // MaxHeap implementation for storing Post objects, ordered by likeCount.
    // If likeCounts are equal, the posts are ordered lexicographically by their id.
    public class MaxHeap {
        private Post[] heap;
        private int size;

        public MaxHeap(int capacity) {
            heap = new Post[capacity];
            size = 0;
        }
        // Inserts a new Post into the heap, resizing the array if necessary.
        public void insert(Post post) {
            if (size == heap.length) {

                resize();
            }
            heap[size] = post;  // Add the new post
            heapifyUp(size);    // Ensure heap properties are maintained
            size++;
        }

        // Removes and returns the Post with the maximum likeCount (root of the heap).
        public Post removeMax() {
            if (size == 0) {
                return null;// Return null if the heap is empty
            }
            Post maxPost = heap[0];// Root post (max)
            Post lastPost = heap[size - 1];
            size--;

            if (size > 0) {
                heap[0] = lastPost;
                heapifyDown(0); // Restore heap order
            }
            return maxPost;
        }

        // Restores the heap order by moving an element up the tree.
        private void heapifyUp(int index) {
            while (index > 0) {
                int parentIndex = (index - 1) / 2;
                Post currentPost = heap[index];
                Post parentPost = heap[parentIndex];
                // Compare likeCount and id to decide swap
                if (currentPost.likeCount > parentPost.likeCount ||
                        (currentPost.likeCount == parentPost.likeCount && currentPost.id.compareTo(parentPost.id) > 0)) {
                    swap(index, parentIndex);// Swap and move up
                    index = parentIndex;
                } else {
                    break;
                }
            }
        }
        // Restores the heap order by moving an element down the tree.
        private void heapifyDown(int index) {
            int largest = index;
            int leftChild = 2 * index + 1;
            int rightChild = 2 * index + 2;
            // Check left child
            if (leftChild < size && (heap[leftChild].likeCount > heap[largest].likeCount ||
                    (heap[leftChild].likeCount == heap[largest].likeCount &&
                            heap[leftChild].id.compareTo(heap[largest].id) > 0))) {
                largest = leftChild;
            }
            // Check right child
            if (rightChild < size && (heap[rightChild].likeCount > heap[largest].likeCount ||
                    (heap[rightChild].likeCount == heap[largest].likeCount &&
                            heap[rightChild].id.compareTo(heap[largest].id) > 0))) {
                largest = rightChild;
            }
            // If the largest isn't the current index, swap and recurse
            if (largest != index) {
                swap(index, largest);
                heapifyDown(largest);
            }
        }
        // Swaps two elements in the heap.
        private void swap(int i, int j) {
            Post temp = heap[i];
            heap[i] = heap[j];
            heap[j] = temp;
        }
        // Resizes the heap array when it's full, doubling its capacity.
        private void resize() {
            Post[] newHeap = new Post[heap.length * 2];
            System.arraycopy(heap, 0, newHeap, 0, heap.length);
            heap = newHeap;
        }

        // Checks if the heap is empty.
        public boolean isEmpty() {
            return size == 0;
        }

    }

}

